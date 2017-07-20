package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Steam extends AbstractPage{

public Steam(WebDriver driver) {
	super(driver);
	PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//a[text()='login']")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='input_username']")
	private WebElement email;
	
	@FindBy(xpath = "//input[@id='input_password']")
	private WebElement password;
	
	@FindBy(xpath = "//button[@type='submit']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//span[@id='account_pulldown']")
	private WebElement dashBoard;
	
	@FindBy(xpath = "//a[text()='Logout']")
	private WebElement logoutBtn;
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//span[@id='account_pulldown']"));
		return isElementPresent(By.xpath("//span[@id='account_pulldown']"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.xpath("//input[@id='input_password']"));
		return isElementPresent(By.xpath("//input[@id='input_password']"));
	}
	

}
