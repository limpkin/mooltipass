package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Cisecurity extends AbstractPage{

	public Cisecurity(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(xpath = "//a[@data-toggle='dropdown']")
	private WebElement userBtn;

	@FindBy(xpath = "//input[@type='login']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='password']")
	private WebElement password;

	@FindBy(xpath = "//button[@type='submit']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//span[@class='fa fa-sign-out']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//button[@data-role='end']")
	private WebElement popUp;
	
	public void goToLogout(){
		waitUntilAppears(userBtn);
		userBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[@data-toggle='dropdown']"));
		return isElementPresent(By.xpath("//a[@data-toggle='dropdown']"));
	}


	public boolean checkAtLoginPage(){
		
		return isElementPresent(By.xpath("//input[@type='login']"));
	}
	
	public void logout(){
		if(isElementPresent(By.xpath("//button[@data-role='end']")))
			popUp.click();
		userBtn.click();
		logoutBtn.click();

	}
}