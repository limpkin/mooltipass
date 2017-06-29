package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Reddit extends AbstractPage{

	public Reddit(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(xpath = "//input[@name='user']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='passwd']")
	private WebElement password;

	@FindBy(xpath = "//button[@type='submit']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[text()='logout']")
	private WebElement logoutBtn;
	

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

		waitUntilAppears(By.xpath("//a[text()='logout']"));
		return isElementPresent(By.xpath("//a[text()='logout']"));
	}

	public void logout(){
		logoutBtn.click();

	}
}
