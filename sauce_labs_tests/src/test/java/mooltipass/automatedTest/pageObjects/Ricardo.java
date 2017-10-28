package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Ricardo extends AbstractPage{

	public Ricardo(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//span[text()='Anmelden']")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@type='text']")
	private WebElement email;

	@FindBy(xpath = "//input[@type='password']")
	private WebElement password;

	@FindBy(xpath = "//input[@type='submit']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[text()='Abmelden']")
	private WebElement logoutBtn;
	
	@FindBy(id = "user-menu")
	private WebElement menuBtn;
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void enterEmail(String value){

		if(waitUntilAppears(email))
				email.sendKeys(value);
		else
			driver.findElement(By.xpath("//input[@name='username']")).sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		if(waitUntilAppears(submitLogin))
			submitLogin.click();
		else
			driver.findElement(By.xpath("//input[@type='submit']")).click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.id("user-menu"));
		return isElementPresent(By.id("user-menu"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@type='password']"));
	}
	
	public void logout(){
		menuBtn.click();
		waitUntilAppears(logoutBtn);
		logoutBtn.click();

	}
}
